interface PageHeaderProps {
  title: string;
  dek: string;
}

export function PageHeader({ title, dek }: PageHeaderProps) {
  return (
    <header>
      <p className="eyebrow">Slab · Field Lab</p>
      <h1 className="page-title">{title}</h1>
      <p className="dek">{dek}</p>
      <hr className="hairline" />
    </header>
  );
}
